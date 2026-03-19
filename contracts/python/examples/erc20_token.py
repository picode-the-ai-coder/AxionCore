"""
ERC20-like Token Smart Contract

A standard token implementation for Axion blockchain.
This demonstrates:
- State management
- Event emission
- Token transfers
- Allowance system

Deploy with:
    axion_node deploy contracts/python/examples/erc20_token.py \
        --name "My Token" \
        --symbol "MYT" \
        --decimals 18 \
        --initial_supply 1000000
"""

from axion_contract import SmartContract
from typing import Dict, Any

class ERC20Token(SmartContract):
    """
    ERC20-like token contract
    
    Standard token with transfer, approve, and allowance functionality
    """
    
    def initialize(self,
                  name: str = "Axion Token",
                  symbol: str = "AXN",
                  decimals: int = 18,
                  initial_supply: int = 1000000) -> Dict[str, Any]:
        """
        Initialize the token
        
        Args:
            name: Token name
            symbol: Token symbol
            decimals: Decimal places
            initial_supply: Initial supply
        
        Returns:
            Initialization result
        """
        # Token metadata
        self.set_state("name", name)
        self.set_state("symbol", symbol)
        self.set_state("decimals", decimals)
        
        # Total supply
        total_supply = initial_supply * (10 ** decimals)
        self.set_state("total_supply", total_supply)
        
        # Give all tokens to deployer
        self.set_state(f"balance_{msg.sender}", total_supply)
        
        # Emit event
        self.emit_event("TokenInitialized", {
            "name": name,
            "symbol": symbol,
            "decimals": decimals,
            "total_supply": total_supply,
            "owner": msg.sender
        })
        
        return {
            "success": True,
            "name": name,
            "symbol": symbol,
            "total_supply": total_supply
        }
    
    def transfer(self, to_address: str, amount: int) -> Dict[str, Any]:
        """
        Transfer tokens to another address
        
        Args:
            to_address: Recipient address
            amount: Amount to transfer
        
        Returns:
            Transfer result
        
        Example:
            result = contract.transfer("0x456def", 100)
        """
        from_address = msg.sender
        
        # Get balances
        from_balance = int(self.get_state(f"balance_{from_address}", 0))
        to_balance = int(self.get_state(f"balance_{to_address}", 0))
        
        # Check balance
        if from_balance < amount:
            return {
                "success": False,
                "error": "Insufficient balance",
                "balance": from_balance,
                "required": amount
            }
        
        # Update balances
        self.set_state(f"balance_{from_address}", from_balance - amount)
        self.set_state(f"balance_{to_address}", to_balance + amount)
        
        # Emit event
        self.emit_event("Transfer", {
            "from": from_address,
            "to": to_address,
            "amount": amount
        })
        
        return {
            "success": True,
            "from": from_address,
            "to": to_address,
            "amount": amount,
            "new_balance": from_balance - amount
        }
    
    def approve(self, spender: str, amount: int) -> Dict[str, Any]:
        """
        Approve another address to spend tokens
        
        Args:
            spender: Address allowed to spend
            amount: Amount allowed
        
        Returns:
            Approval result
        
        Example:
            result = contract.approve("0x789ghi", 500)
        """
        owner = msg.sender
        
        # Set allowance
        self.set_state(f"allowance_{owner}_{spender}", amount)
        
        # Emit event
        self.emit_event("Approval", {
            "owner": owner,
            "spender": spender,
            "amount": amount
        })
        
        return {
            "success": True,
            "owner": owner,
            "spender": spender,
            "amount": amount
        }
    
    def transfer_from(self,
                     from_address: str,
                     to_address: str,
                     amount: int) -> Dict[str, Any]:
        """
        Transfer tokens on behalf of another address
        
        Requires prior approval
        
        Args:
            from_address: Token owner
            to_address: Recipient
            amount: Amount to transfer
        
        Returns:
            Transfer result
        """
        spender = msg.sender
        
        # Check allowance
        allowance = int(self.get_state(
            f"allowance_{from_address}_{spender}", 0
        ))
        if allowance < amount:
            return {
                "success": False,
                "error": "Insufficient allowance"
            }
        
        # Check balance
        balance = int(self.get_state(f"balance_{from_address}", 0))
        if balance < amount:
            return {
                "success": False,
                "error": "Insufficient balance"
            }
        
        # Update balances
        from_balance = int(self.get_state(f"balance_{from_address}", 0))
        to_balance = int(self.get_state(f"balance_{to_address}", 0))
        
        self.set_state(f"balance_{from_address}", from_balance - amount)
        self.set_state(f"balance_{to_address}", to_balance + amount)
        
        # Update allowance
        self.set_state(
            f"allowance_{from_address}_{spender}",
            allowance - amount
        )
        
        # Emit event
        self.emit_event("Transfer", {
            "from": from_address,
            "to": to_address,
            "amount": amount,
            "spender": spender
        })
        
        return {
            "success": True,
            "from": from_address,
            "to": to_address,
            "amount": amount
        }
    
    def balance_of(self, address: str) -> Dict[str, Any]:
        """
        Get token balance of an address
        
        Args:
            address: Account address
        
        Returns:
            Balance information
        
        Example:
            result = contract.balance_of("0x123abc")
        """
        balance = int(self.get_state(f"balance_{address}", 0))
        return {
            "address": address,
            "balance": balance
        }
    
    def allowance(self, owner: str, spender: str) -> Dict[str, Any]:
        """
        Get allowance for a spender
        
        Args:
            owner: Token owner
            spender: Spender address
        
        Returns:
            Allowance information
        """
        amount = int(self.get_state(
            f"allowance_{owner}_{spender}", 0
        ))
        return {
            "owner": owner,
            "spender": spender,
            "allowance": amount
        }
    
    def total_supply(self) -> Dict[str, Any]:
        """Get total token supply"""
        supply = int(self.get_state("total_supply", 0))
        return {"total_supply": supply}
    
    def name(self) -> Dict[str, str]:
        """Get token name"""
        return {"name": self.get_state("name")}
    
    def symbol(self) -> Dict[str, str]:
        """Get token symbol"""
        return {"symbol": self.get_state("symbol")}
    
    def decimals(self) -> Dict[str, int]:
        """Get decimal places"""
        return {"decimals": int(self.get_state("decimals"))}
    
    def mint(self, to_address: str, amount: int) -> Dict[str, Any]:
        """
        Mint new tokens (owner only)
        
        Args:
            to_address: Recipient
            amount: Amount to mint
        
        Returns:
            Mint result
        """
        owner = self.get_state("owner")
        self.require(msg.sender == owner, "Only owner can mint")
        
        # Update supply
        supply = int(self.get_state("total_supply", 0))
        self.set_state("total_supply", supply + amount)
        
        # Update balance
        balance = int(self.get_state(f"balance_{to_address}", 0))
        self.set_state(f"balance_{to_address}", balance + amount)
        
        # Emit event
        self.emit_event("Mint", {
            "to": to_address,
            "amount": amount
        })
        
        return {
            "success": True,
            "to": to_address,
            "amount": amount,
            "new_supply": supply + amount
        }
    
    def burn(self, amount: int) -> Dict[str, Any]:
        """
        Burn tokens (remove from circulation)
        
        Args:
            amount: Amount to burn
        
        Returns:
            Burn result
        """
        burner = msg.sender
        balance = int(self.get_state(f"balance_{burner}", 0))
        
        self.require(balance >= amount, "Insufficient balance to burn")
        
        # Update balance
        self.set_state(f"balance_{burner}", balance - amount)
        
        # Update supply
        supply = int(self.get_state("total_supply", 0))
        self.set_state("total_supply", supply - amount)
        
        # Emit event
        self.emit_event("Burn", {
            "burner": burner,
            "amount": amount
        })
        
        return {
            "success": True,
            "burned": amount,
            "new_supply": supply - amount
        }
